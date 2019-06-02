#include "9cc.h"

// エラーを報告するための関数
noreturn void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char *format(char *fmt, ...) {
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return strdup(buf);
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
    ty->size = base->size * size;
    ty->align = base->align;
    ty->array_size = size;
    ty->array_of = base;
    return ty;
}

Type *ptr_to(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = PTR;
    ty->ptr_to = base;
    ty->size = 8;
    ty->align = 8;
    return ty;
}

int roundup(int x, int align) { return (x + align - 1) & ~(align - 1); }

Type *struct_of(Vector *members) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = STRUCT;
    ty->members = new_vector();

    int off = 0;
    for (int i = 0; i < members->len; i++) {
        Node *node = members->data[i];
        assert(node->op == ND_VARDEF);

        Type *t = node->ty;
        off = roundup(off, t->align);
        t->offset = off;
        off += t->size;

        if (ty->align < node->ty->align)
            ty->align = node->ty->align;
    }
    ty->size = roundup(off, ty->align);
    return ty;
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp)
        error("cannnot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = malloc(size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}
