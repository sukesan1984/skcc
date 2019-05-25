#include "9cc.h"

// エラーを報告するための関数
noreturn void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 可変長Vector
// 任意蝶の入力をサポートする
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

Map *new_map() {
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, char *key, void *val) {
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
    for (int i = map->keys->len -1; i >= 0; i--)
        if (strcmp(map->keys->data[i], key) == 0)
            return map->vals->data[i];
    return NULL;
}

bool map_exists(Map *map, char *key) {
    for (int i = map->keys->len -1; i >= 0; i--)
        if (strcmp(map->keys->data[i], key) == 0)
            return true;
    return false;
}

Type *ary_of(Type *base, size_t size) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = ARRAY;
    ty->array_size = size;
    ty->array_of = base;
    return ty;
}

Type *ptr_of(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = PTR;
    ty->ptr_of = base;
    return ty;
}

int size_of(Type *ty) {
    if (ty->ty == INT)
        return 4;
    if (ty->ty == ARRAY)
        return size_of(ty->array_of) * ty->array_size;
    assert(ty->ty == PTR);
    return 8;
}
