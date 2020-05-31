//#include "9cc.h"
typedef struct Hoge {
    int x;
    int y;
} Hoge;
typedef struct {
    void **data;  // 実際のデータ
    int capacity; // バッファの大きさ
    int len;      // ベクタに追加済みの要素の個数。len == capacityのときにバッファがいっぱい、新たに要素を足す場合は、新たにバッファを確保して既存の要素をコピーし、dataポインタをすげ替える
} Vector;


Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec-> data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

int main () {
    struct Hoge* hoge = malloc(sizeof(Hoge));
    struct Hoge* fuga = malloc(sizeof(Hoge));
    Vector *vec = new_vector();
    hoge->x = 1;
    hoge->y = 2;
    fuga->x = 3;
    fuga->y = 4;
    vec_push(vec, hoge);
    vec_push(vec, fuga);
    for (int i = 0; i < vec->len; i++) {
        printf("%d\n", ((Hoge*)vec->data[i])->x);
        printf("%d\n", ((Hoge*)vec->data[i])->y);
    }

    return 0;
}
