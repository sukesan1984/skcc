char *g6[] = {"abc", "efg"};
char *g5 = "hij";
struct { char a; int b; } g9[2] = {{1, 2}, {3, 4}};
//int g[] = {1, 2, 3};
int main () {
    printf("%d\n", g9[0].a);
    printf("%d\n", g9[0].b);
    //printf("%s\n", g5);
    //printf("%s\n", g6[0]);
    //printf("%s\n", g6[1]);
    //printf("%d\n", g[0]);
    //printf("%d\n", g[1]);
    //printf("%d\n", g[2]);
    return 0;
}
