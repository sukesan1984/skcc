//#include <stdio.h>
int main () {
#if 0
#include "/no/such/file"
    assert(0, 1, "1");
#if nest
#endif
#endif
    printf("%d, %s\n", 1
#if 1
            +2
#endif
            , "1 + 2");
    return 0;
}
