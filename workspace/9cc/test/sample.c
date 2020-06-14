//#include <stdio.h> //#include <stdlib.h>

typedef struct PlaceHolder {
    int val;
} PlaceHolder;

void    nop() {}
extern void *calloc (int __nmemb, int __size);

int main() {
    struct tag {
        char a;
        int b;
    } x;
    struct tag *p = &x;
    x.a = 3;
    x.b = 5;
    nop();
    printf("%d\n", p->a + p->b);

    nop();
    PlaceHolder *placeholder = calloc(1, sizeof(PlaceHolder));
    nop();
    PlaceHolder replaced;
    replaced.val = 3;
    *placeholder = replaced;
    printf("val: %d\n", placeholder->val);
    return 0;
}
